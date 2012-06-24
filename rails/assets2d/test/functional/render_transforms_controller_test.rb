require 'test_helper'

class RenderTransformsControllerTest < ActionController::TestCase
  setup do
    @render_transform = render_transforms(:one)
  end

  test "should get index" do
    get :index
    assert_response :success
    assert_not_nil assigns(:render_transforms)
  end

  test "should get new" do
    get :new
    assert_response :success
  end

  test "should create render_transform" do
    assert_difference('RenderTransform.count') do
      post :create, render_transform: {  }
    end

    assert_redirected_to render_transform_path(assigns(:render_transform))
  end

  test "should show render_transform" do
    get :show, id: @render_transform
    assert_response :success
  end

  test "should get edit" do
    get :edit, id: @render_transform
    assert_response :success
  end

  test "should update render_transform" do
    put :update, id: @render_transform, render_transform: {  }
    assert_redirected_to render_transform_path(assigns(:render_transform))
  end

  test "should destroy render_transform" do
    assert_difference('RenderTransform.count', -1) do
      delete :destroy, id: @render_transform
    end

    assert_redirected_to render_transforms_path
  end
end
