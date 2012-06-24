require 'test_helper'

class ShotsControllerTest < ActionController::TestCase
  setup do
    @shot = shots(:one)
  end

  test "should get index" do
    get :index
    assert_response :success
    assert_not_nil assigns(:shots)
  end

  test "should get new" do
    get :new
    assert_response :success
  end

  test "should create shot" do
    assert_difference('Shot.count') do
      post :create, shot: {  }
    end

    assert_redirected_to shot_path(assigns(:shot))
  end

  test "should show shot" do
    get :show, id: @shot
    assert_response :success
  end

  test "should get edit" do
    get :edit, id: @shot
    assert_response :success
  end

  test "should update shot" do
    put :update, id: @shot, shot: {  }
    assert_redirected_to shot_path(assigns(:shot))
  end

  test "should destroy shot" do
    assert_difference('Shot.count', -1) do
      delete :destroy, id: @shot
    end

    assert_redirected_to shots_path
  end
end
