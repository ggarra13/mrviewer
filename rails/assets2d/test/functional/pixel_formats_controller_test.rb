require 'test_helper'

class PixelFormatsControllerTest < ActionController::TestCase
  setup do
    @pixel_format = pixel_formats(:one)
  end

  test "should get index" do
    get :index
    assert_response :success
    assert_not_nil assigns(:pixel_formats)
  end

  test "should get new" do
    get :new
    assert_response :success
  end

  test "should create pixel_format" do
    assert_difference('PixelFormat.count') do
      post :create, pixel_format: {  }
    end

    assert_redirected_to pixel_format_path(assigns(:pixel_format))
  end

  test "should show pixel_format" do
    get :show, id: @pixel_format
    assert_response :success
  end

  test "should get edit" do
    get :edit, id: @pixel_format
    assert_response :success
  end

  test "should update pixel_format" do
    put :update, id: @pixel_format, pixel_format: {  }
    assert_redirected_to pixel_format_path(assigns(:pixel_format))
  end

  test "should destroy pixel_format" do
    assert_difference('PixelFormat.count', -1) do
      delete :destroy, id: @pixel_format
    end

    assert_redirected_to pixel_formats_path
  end
end
